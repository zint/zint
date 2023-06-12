/*	ocrb_woff2.h - OCR-B (EAN/UPC subset) as base 64 string */
/*
    libzint - the open source barcode library
    Copyright (C) 2023 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef Z_UPCEAN_WOFF2_H
#define Z_UPCEAN_WOFF2_H

/* Adapted from OCR-B font version 0.2 Matthew Skala
 *   https://tsukurimashou.osdn.jp/ocr.php.en
 *
 * Copyright Matthew Skala (2011); based on code by Norbert Schwarz (1986, 2011)
 *
 * "The version in this package descends from a set of Metafont
 *  definitions by Norbert Schwarz of Ruhr-Universitaet Bochum,
 *  bearing dates ranging from 1986 to 2010. He originally
 *  distributed it under a "non-commercial use only"
 *  restriction but has since released it for unrestricted use
 *  and distribution. See the README file for more details."
 *
 * The README states (http://mirrors.ctan.org/fonts/ocr-b.zip)
 *
 * "As far as the digitization in METAFONT input which I have
 *  developed, you may freely use, modify, and/or distribute any of
 *  these files or the resulting fonts, without limitation.  A previous
 *  release of ocr-b only granted rights for non-commercial use; that
 *  restriction is now lifted."
 */
static const char upcean_woff2[1905] =
	"d09GMgABAAAAAAWUAA4AAAAACWwAAAVAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGhYbIBwqBlYARBEICoosiEMLHAABNgIkAxwEIAWCdgcgG3A"
	"HUZQNToDsZ0Lmpodml0MocYiJWq3MrpVLZEJ+vN3Pv5v+uQSv2A20nhmlog7VlNREyOZ0r49OVZEJPLVfUR54bvk7Los0CqTnCfSi2iEa0i67dW"
	"964kDG5b9/ecD9z/18fqPNwoZRuAHOhvii7ruLDKo8XiYbDftHU5QXfpIPyF9jbUb2MTDyZycgAJasIQ4EseR/EICYK3eoSVfQIMQFAP8DAIDSy"
	"8hx1rMeage6wBFgoBvDB9LiAc4UHgCsLrPoC9W0EnDChcgR+dP8Pq0BEgWKj6g5Ull0JGhpggOEqscICLhYA0BION+IEiv+Es7AeuV899KL7QFR"
	"GCpolRPO03YAbQGnpme53yJDTOskRg96rjuWBPp7yp0t3SUWNo5WpKScNUaWb40WM/rXeiWoddapmnVizQGxxlBl1MXrLeajBGLV6J3wDeOhV9S"
	"ualbKbNR3HAau+pDmCOLRrOIRFzxXoLm3XNNdirx06dUpV6tljfqQ2HIrYj3OGhljwEnEGkJJgUYZ9qjmJR+H5Bxj5Y+wUoaN8imRbQBqTOv1AD"
	"rddOyB4QYdk5ZvhOHwpK/EAqBVtFg9IIYzADIk44q2zFHExt8RaxYWEGK2rkYjSF/NS+vcSQaxzEnj26KNalJHHzx3VWNg1PoGjZcJ6qqvQbEXD"
	"ul8dQcwInmfky6Wpy8xBsQ+V1JkQVosyuj7VLpy9fHL+xcdPHj1wdRGPCbSFq57asnI7BMfFmR26qWPrLT5RLGa2vUyOY5Kmhi9UF+3Ab0WNMp+"
	"3CB6QY8qhVtu0z4vvbLILSoy/Rcr52XYtnErktMc+nyai+LXEVoOj+JlNoaHNkpG6pU/NJJzMoka/KO52falZ2WsLO72/V/h0zDpzxe2fbSnsBk"
	"fwma5iVSpO59i8jmw212V5bEjtVOD1TLnNMvEcM/0ES9U4eRHPpebcQ02YzJvzq2qOsOvNn7NHllIelxY5ZNLdtgMPamkCQvO/pyZ25JJpIsyhz"
	"KKsVl+5OItKUfsGdkevniB/W87DpZKOgbLbLide9TDbvvTre9Aj95HlPPuRY7WernJIuvpmesDQqQaGAjbMRA20XvzxO/YBBobbxN+Edtu455Ov"
	"B6c9xUXf3QgrP5yuIJl2Kt+F2htVu5kp5ppRUPpAwzfWE6l9d8oaJmKfHGFn3J4MTgdrtBTA8tmhtpnlptWLq3pSC7ISU0pVCQpcgqyM5VFcJd8"
	"RmrJ51gZwzi3sk3Jg9ffZ8+7vJnRzpXCZf30Qz/74YS2riu5S7Gkhi5aWpsTF1WYqK2bKBiaUPF9O1cP1VWGFcckkWE5yf4VaxWblPNDHR0+ufn"
	"s2YtJ7Lgge1Fh8tmA2uY2tbTPzg5kevXJkmntRPcv9tjk/X9daVrn3vk9q1bVNyXkK9NGHFK1nlkFxakJqkqd0NP5P9tkvSmxl3X+sSTH9xvZeG"
	"0vjet61vhzY/vnmDRhm0cbrZ9Sl3d56WaDn7+ICsCjqsbibdU2sX+hRfQDAM+mkCL/hMOfisroCPidGAGyT85klYVvCbmjApoDP+cQgF5ai3N1r"
	"xneYEC9FQZP4pjMOJwouk2gDAHFBYDloDUiQsuNHNZmjYRC24xczn4z8uBAiHyx36FUa8CAFo2GUYq0q9WhFkUuWKAgQbzEoNSp1a9RA0q3LpR6"
	"3Ro0zsOoD7l96GuhMceBjbT1WgyTKjEGcnlzlEjhfFEb2+ZJUShZYaPZoA4Y0w/AgRL65rARB3/TSIlGAWXOo0yWmg4WwU8Uv0OWDYZRdd1Uv9Z"
	"HQXBqJgAAgewAAAAA";

/* vim: set ts=4 sw=4 et : */
#endif /* Z_UPCEAN_WOFF2_H */
