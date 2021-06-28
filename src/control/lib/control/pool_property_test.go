//
// (C) Copyright 2021 Intel Corporation.
//
// SPDX-License-Identifier: BSD-2-Clause-Patent
//

package control_test

import (
	"testing"

	"github.com/google/go-cmp/cmp"
	"github.com/pkg/errors"

	"github.com/daos-stack/daos/src/control/common"
	"github.com/daos-stack/daos/src/control/lib/control"
)

func TestControl_PoolPropertyValue(t *testing.T) {
	strPtr := func(in string) *string {
		return &in
	}
	numPtr := func(in uint64) *uint64 {
		return &in
	}

	for name, tc := range map[string]struct {
		val    *control.PoolPropertyValue
		strVal *string
		numVal *uint64
		expErr error
		expStr string
	}{
		"nil": {
			expErr: errors.New("not set"),
			expStr: "value not set",
		},
		"not set": {
			val:    &control.PoolPropertyValue{},
			expErr: errors.New("not set"),
			expStr: "value not set",
		},
		"string value": {
			val:    &control.PoolPropertyValue{},
			strVal: strPtr("hi"),
			expErr: errors.New("not uint64"),
			expStr: "hi",
		},
		"number value": {
			val:    &control.PoolPropertyValue{},
			numVal: numPtr(42),
			expStr: "42",
		},
	} {
		t.Run(name, func(t *testing.T) {
			v := tc.val

			if tc.strVal != nil {
				v.SetString(*tc.strVal)
			} else if tc.numVal != nil {
				v.SetNumber(*tc.numVal)
			}

			gotStr := v.String()
			if diff := cmp.Diff(tc.expStr, gotStr); diff != "" {
				t.Fatalf("unexpected String() result (-want, +got):\n%s\n", diff)
			}

			gotNum, gotErr := v.GetNumber()
			common.CmpErr(t, tc.expErr, gotErr)
			if tc.expErr != nil {
				return
			}
			common.AssertEqual(t, *tc.numVal, gotNum, "unexpected GetNumber() result")
		})
	}
}

func TestControl_PoolProperties(t *testing.T) {
	for name, tc := range map[string]struct {
		name    string
		value   string
		expErr  error
		expStr  string
		expJson []byte
	}{
		"label-valid": {
			name:    "label",
			value:   "valid",
			expStr:  "label:valid",
			expJson: []byte(`{"name":"label","description":"Pool label","value":"valid"}`),
		},
		"label-invalid": {
			name:   "label",
			value:  "bad label",
			expErr: errors.New("invalid label"),
		},
		"reclaim-valid": {
			name:    "reclaim",
			value:   "lazy",
			expStr:  "reclaim:lazy",
			expJson: []byte(`{"name":"reclaim","description":"Reclaim strategy","value":"lazy"}`),
		},
		"reclaim-invalid": {
			name:   "reclaim",
			value:  "wat",
			expErr: errors.New("invalid"),
		},
		"ec_cell_sz-valid": {
			name:    "ec_cell_sz",
			value:   "1MiB",
			expStr:  "ec_cell_sz:1.0 MiB",
			expJson: []byte(`{"name":"ec_cell_sz","description":"EC cell size","value":1048576}`),
		},
		"ec_cell_sz-invalid": {
			name:   "ec_cell_sz",
			value:  "wat",
			expErr: errors.New("invalid"),
		},
		"space_rb-valid": {
			name:    "space_rb",
			value:   "25",
			expStr:  "space_rb:25%",
			expJson: []byte(`{"name":"space_rb","description":"Rebuild space ratio","value":25}`),
		},
		"space_rb-invalid": {
			name:   "space_rb",
			value:  "wat",
			expErr: errors.New("invalid"),
		},
		"space_rb-gt100": {
			name:   "space_rb",
			value:  "101",
			expErr: errors.New("invalid"),
		},
		"self_heal-valid": {
			name:    "self_heal",
			value:   "exclude",
			expStr:  "self_heal:auto-exclude,manual-rebuild",
			expJson: []byte(`{"name":"self_heal","description":"Self-healing policy","value":"auto-exclude,manual-rebuild"}`),
		},
		"self_heal-invalid": {
			name:   "self_heal",
			value:  "wat",
			expErr: errors.New("invalid"),
		},
	} {
		t.Run(name, func(t *testing.T) {
			prop, err := control.PoolProperties().GetProperty(tc.name)
			if err != nil {
				t.Fatal(err)
			}
			gotErr := prop.SetValue(tc.value)
			common.CmpErr(t, tc.expErr, gotErr)
			if tc.expErr != nil {
				return
			}

			if diff := cmp.Diff(tc.expStr, prop.String()); diff != "" {
				t.Fatalf("unexpected String() value (-want, +got):\n%s\n", diff)
			}
			gotJson, err := prop.MarshalJSON()
			if err != nil {
				t.Fatal(err)
			}
			if diff := cmp.Diff(tc.expJson, gotJson); diff != "" {
				t.Fatalf("unexpected MarshalJSON() value (-want, +got):\n%s\n", diff)
			}
		})
	}
}
