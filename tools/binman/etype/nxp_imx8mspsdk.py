# SPDX-License-Identifier: GPL-2.0+
"""SPSDK-backed HAB4 signing for i.MX8M binman images.

The i.MX8M image layout is intentionally identical to the existing CST entry:
binman constructs the SPL/FIT payload and this entry adds its IVT and a 0x2000
byte HAB reservation.  SPSDK generates only the CSF, avoiding its unrelated
full-image layouts.
"""

import os
import struct

from binman.etype.mkimage import Entry_mkimage
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

MAGIC_NXP_IMX_IVT = 0x412000D1
MAGIC_FITIMAGE = 0xEDFE0DD0


class Entry_nxp_imx8mspsdk(Entry_mkimage):
    """Generate an i.MX8M HAB4 CSF using the open-source SPSDK.

    SPSDK_HAB_* variables take precedence over the existing SRK_TABLE,
    CSF_KEY, IMG_KEY, CSF_SIGNER and IMG_SIGNER variables. The corresponding
    nxp,* device-tree properties are used as a final fallback for inputs that
    are not supplied by the environment. SPSDK_HAB_CSF_SIGNER and
    SPSDK_HAB_IMG_SIGNER (or their legacy/DT equivalents) must identify the
    private-key providers; unlike CST, SPSDK cannot infer encrypted key paths.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ["nxp,loader-address"]
        self.loader_address = None
        self.srk_table = None
        self.csf_crt = None
        self.img_crt = None
        self.csf_signer = None
        self.img_signer = None
        self.unlock = False
        self.spsdk = None

    def ReadNode(self):
        """Read the HAB4 signing inputs from the device tree and environment."""
        super().ReadNode()
        self.loader_address = fdt_util.GetInt(self._node, "nxp,loader-address")
        self.srk_table = os.getenv(
            "SPSDK_HAB_SRK_TABLE",
            os.getenv(
                "SRK_TABLE",
                fdt_util.GetString(
                    self._node, "nxp,srk-table", "SRK_1_2_3_4_table.bin"
                ),
            ),
        )
        self.csf_crt = os.getenv(
            "SPSDK_HAB_CSF_CERT",
            os.getenv(
                "CSF_KEY",
                fdt_util.GetString(
                    self._node, "nxp,csf-crt", "CSF1_1_sha256_2048_65537_v3_usr_crt.pem"
                ),
            ),
        )
        self.img_crt = os.getenv(
            "SPSDK_HAB_IMG_CERT",
            os.getenv(
                "IMG_KEY",
                fdt_util.GetString(
                    self._node, "nxp,img-crt", "IMG1_1_sha256_2048_65537_v3_usr_crt.pem"
                ),
            ),
        )
        self.csf_signer = os.getenv(
            "SPSDK_HAB_CSF_SIGNER",
            os.getenv("CSF_SIGNER", fdt_util.GetString(self._node, "nxp,csf-signer")),
        )
        self.img_signer = os.getenv(
            "SPSDK_HAB_IMG_SIGNER",
            os.getenv("IMG_SIGNER", fdt_util.GetString(self._node, "nxp,img-signer")),
        )
        self.unlock = fdt_util.GetBool(self._node, "nxp,unlock")
        self.ReadEntries()

    def _check_signing_inputs(self):
        inputs = {
            "SRK table": self.srk_table,
            "CSF certificate": self.csf_crt,
            "IMG certificate": self.img_crt,
            "CSF signer": self.csf_signer,
            "IMG signer": self.img_signer,
        }
        missing = [name for name, value in inputs.items() if not value]
        if missing:
            self.Raise(
                "Missing SPSDK HAB4 input(s): %s; provide SPSDK_HAB_* "
                "variables, legacy variables, or nxp,* device-tree properties"
                % ", ".join(missing)
            )

    def _make_config(self, uniq, input_fname):
        self._check_signing_inputs()
        cfg_fname = tools.get_output_filename(f"nxp.spsdk-hab4.{uniq}.yaml")
        unlock = (
            """\n  - Unlock:\n      Unlock_Engine: CAAM\n      Unlock_Features:\n        - MID"""
            if self.unlock
            else ""
        )
        config = f"""options:
  flags: 0x8
  startAddress: {self.loader_address:#x}
  ivtOffset: 0
  initialLoadSize: 0
inputImageFile: "{input_fname}"
sections:
  - Header:
      Header_Version: "4.3"
      Header_HashAlgorithm: sha256
      Header_Engine: CAAM
      Header_EngineConfiguration: 0
      Header_CertificateFormat: x509
      Header_SignatureFormat: CMS
  - InstallSRK:
      InstallSRK_Table: "{self.srk_table}"
      InstallSRK_SourceIndex: 0
  - InstallCSFK:
      InstallCSFK_File: "{self.csf_crt}"
      InstallCSFK_CertificateFormat: x509
  - AuthenticateCSF:
      AuthenticateCSF_CertificateFormat: x509
      AuthenticateCSF_SignatureFormat: CMS
      Signer: "{self.csf_signer}"{unlock}
  - InstallKey:
      InstallKey_File: "{self.img_crt}"
      InstallKey_VerificationIndex: 0
      InstallKey_TargetIndex: 2
  - AuthenticateData:
      AuthenticateData_VerificationIndex: 2
      AuthenticateData_Engine: CAAM
      AuthenticateData_EngineConfiguration: 0
      Signer: "{self.img_signer}"
"""
        tools.write_file(cfg_fname, config.encode())
        return cfg_fname

    def BuildSectionData(self, required):
        """Build and sign the image section data."""
        data, _, uniq = self.collect_contents_to_file(self._entries.values(), "input")
        signtype = struct.unpack("<I", data[:4])[0]
        signbase = self.loader_address
        if signtype == MAGIC_NXP_IMX_IVT:
            signbase -= 0x40
            signsize = struct.unpack("<I", data[24:28])[0] - signbase
            data = data[:signsize]
        elif signtype == MAGIC_FITIMAGE:
            signsize = tools.align(len(data), 0x1000)
            data += tools.get_bytes(0, signsize - len(data))
            data += struct.pack(
                "<8I",
                MAGIC_NXP_IMX_IVT,
                signbase + signsize,
                0,
                0,
                0,
                signbase + signsize,
                signbase + signsize + 0x20,
                0,
            )
        else:
            return data

        input_fname = tools.get_output_filename(f"nxp.spsdk-hab4-input.{uniq}")
        tools.write_file(input_fname, data)
        config_fname = self._make_config(uniq, input_fname)
        output_fname = tools.get_output_filename(f"nxp.spsdk-hab4-csf.{uniq}")
        if (
            self.spsdk.run(config_fname, input_fname, output_fname, signbase)
            is not None
        ):
            return data + tools.read_file(output_fname)
        self.record_missing_bintool(self.spsdk)
        return data

    def SetImagePos(self, image_pos):
        """Set the image position after sizing the child entries."""
        upto = 0
        for entry in super().GetEntries().values():
            entry.SetOffsetSize(upto, None)
            if entry.size is None:
                return
            upto += entry.size
        Entry_section.SetImagePos(self, image_pos)

    def AddBintools(self, btools):
        """Register the SPSDK HAB4 signing bintool."""
        super().AddBintools(btools)
        self.spsdk = self.AddBintool(btools, "spsdk_hab4_sign")
