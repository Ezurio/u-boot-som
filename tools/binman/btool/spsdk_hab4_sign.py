# SPDX-License-Identifier: GPL-2.0+
"""Bintool implementation for the Summit SPSDK HAB4 adapter."""

from binman import bintool


class Bintoolspsdk_hab4_sign(bintool.Bintool):
    """Generate a HAB4 CSF with SPSDK."""

    def __init__(self, name):
        super().__init__('spsdk-hab4-sign.py', 'Generate NXP HAB4 CSF with SPSDK')

    def run(self, config, input_fname, output_fname, address):
        """Generate a CSF blob for an image loaded at *address*."""
        return self.run_cmd(
            '--config', config,
            '--input', input_fname,
            '--output', output_fname,
            '--address', f'{address:#x}',
        )
