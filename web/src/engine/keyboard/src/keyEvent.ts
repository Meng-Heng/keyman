// TODO:  Move to separate folder:  'codes'
// We should start splitting off code needed by keyboards even without a KeyboardProcessor active.
// There's an upcoming `/common/web/types` package that 'codes' and 'keyboards' may fit well within.

// KeyEvent may be a _little_ bit of pollution, but this IS what the Web OSK currently generates to signal
// a key event.  The most straightforward way to integrate Web OSK events on other platforms is to have
// other platforms recognize and utilize this type.

import type Keyboard from "./keyboards/keyboard.js";
import { type DeviceSpec } from "@keymanapp/web-utils";

import Codes from './codes.js';
import DefaultRules from "./defaultRules.js";
import { ActiveKeyBase } from './keyboards/activeLayout.js';

// Represents a probability distribution over a keyboard's keys.
// Defined here to avoid compilation issues.
export type KeyDistribution = { keySpec: ActiveKeyBase, p: number }[];

/**
 * A simple instance of the standard 'default rules' for keystroke processing from the
 * DefaultRules base class.
 */
const BASE_DEFAULT_RULES = new DefaultRules();

export interface KeyEventSpec {

  Lcode: number;
  Lstates: number;
  LmodifierChange?: boolean;
  Lmodifiers: number;
  LisVirtualKey?: boolean;
  vkCode?: number;
  kName: string;
  kLayer?: string;   // The key's layer property
  kbdLayer?: string; // The virtual keyboard's active layer
  kNextLayer?: string;

  /**
   * Marks the active keyboard at the time that this KeyEvent was generated by the user.
   *
   * Note:  this is NOT equivalent to the active keyboard at the time that the event handler begins
   * processing!  It should be set via closure (or similar) on the event handler that can 100%
   * guarantee that the keyboard instance known to the handler has not changed during JS execution
   * since the user's interaction that raised the event.
   */
  srcKeyboard?: Keyboard;

  // Holds a generated fat-finger distribution (when appropriate)
  keyDistribution?: KeyDistribution;

  /**
   * The device model for web-core to follow when processing the keystroke.
   */
  device: DeviceSpec;

  /**
   * `true` if this event was produced by sources other than a DOM-based KeyboardEvent.
   */
  isSynthetic?: boolean;
}

/**
 * This class is defined within its own file so that it can be loaded by code outside of KMW without
 * having to actually load the entirety of KMW.
 */
export default class KeyEvent implements KeyEventSpec {
  Lcode: number;
  Lstates: number;
  LmodifierChange?: boolean;
  Lmodifiers: number;
  LisVirtualKey?: boolean;
  vkCode?: number;
  kName: string;
  kLayer?: string;   // The key's layer property
  kbdLayer?: string; // The virtual keyboard's active layer
  kNextLayer?: string;
  baseTranscriptionToken?: number;

  /**
   * Marks the active keyboard at the time that this KeyEvent was generated by the user.
   *
   * Note:  this is NOT equivalent to the active keyboard at the time that the event handler begins
   * processing!  It should be set via closure (or similar) on the event handler that can 100%
   * guarantee that the keyboard instance known to the handler has not changed during JS execution
   * since the user's interaction that raised the event.
   */
  srcKeyboard?: Keyboard;

  // Holds relevant event properties leading to construction of this KeyEvent.
  source?: any; // Technically, KeyEvent|MouseEvent|Touch - but those are DOM types that must be kept out of headless mode.
  // Holds a generated fat-finger distribution (when appropriate)
  keyDistribution?: KeyDistribution;

  /**
   * The device model for web-core to follow when processing the keystroke.
   */
  device: DeviceSpec;

  /**
   * `true` if this event was produced by sources other than a DOM-based KeyboardEvent.
   */
  isSynthetic: boolean = true;

  public constructor(keyEventSpec: KeyEventSpec) {
    for(let key in keyEventSpec) {
      // @ts-ignore
      if(keyEventSpec[key] !== undefined) {
        // @ts-ignore
        this[key] = keyEventSpec[key];
      }
    }
  }

  public static constructNullKeyEvent(device: DeviceSpec): KeyEvent {
    const keyEvent = new KeyEvent({
      Lcode: 0,
      kName: '',
      device: device,
      Lstates: undefined,
      Lmodifiers: undefined,
      vkCode: undefined,
      LisVirtualKey: undefined
    });
    return keyEvent;
  }

  get isModifier(): boolean {
    switch(this.Lcode) {
      case 16: //"K_SHIFT":16,"K_CONTROL":17,"K_ALT":18
      case 17:
      case 18:
      case 20: //"K_CAPS":20, "K_NUMLOCK":144,"K_SCROLL":145
      case 144:
      case 145:
        return true;
      default:
        return false;
    }
  }

  // FIXME:  makes some bad assumptions.
  setMnemonicCode(shifted: boolean, capsActive: boolean) {
    // K_SPACE is not handled by defaultKeyOutput for physical keystrokes unless using touch-aliased elements.
    // It's also a "exception required, March 2013" for clickKey, so at least they both have this requirement.
    if(this.Lcode != Codes.keyCodes['K_SPACE']) {
      // So long as the key name isn't prefixed with 'U_', we'll get a default mapping based on the Lcode value.
      // We need to determine the mnemonic base character - for example, SHIFT + K_PERIOD needs to map to '>'.
      let mappingEvent: KeyEvent = new KeyEvent(this);
      for(let key in (this as KeyEvent)) {
        // @ts-ignore
        mappingEvent[key as keyof KeyEvent] = this[key];
      }

      // To facilitate storing relevant commands, we should probably reverse-lookup
      // the actual keyname instead.
      mappingEvent.kName = 'K_xxxx';
      mappingEvent.Lmodifiers = (shifted ? 0x10 : 0);  // mnemonic lookups only exist for default & shift layers.
      var mappedChar: string = BASE_DEFAULT_RULES.forAny(mappingEvent, true);

      /* First, save a backup of the original code.  This one won't needlessly trigger keyboard
        * rules, but allows us to replicate/emulate commands after rule processing if needed.
        * (Like backspaces)
        */
      this.vkCode = this.Lcode;
      if(mappedChar) {
        // Will return 96 for 'a', which is a keycode corresponding to Codes.keyCodes('K_NP1') - a numpad key.
        // That stated, we're in mnemonic mode - this keyboard's rules are based on the char codes.
        this.Lcode = mappedChar.charCodeAt(0);
      } else {
        // Don't let command-type keys (like K_DEL, which will output '.' otherwise!)
        // trigger keyboard rules.
        //
        // However, DO make sure modifier keys pass through safely.
        // (https://github.com/keymanapp/keyman/issues/3744)
        if(!this.isModifier) {
          delete this.Lcode;
        }
      }
    }

    if(capsActive) {
      // TODO:  Needs fixing - does not properly mirror physical keystrokes, as Lcode range 96-111 corresponds
      // to numpad keys!  (Physical keyboard section has its own issues here.)
      if((this.Lcode >= 65 && this.Lcode <= 90) /* 'A' - 'Z' */ || (this.Lcode >= 97 && this.Lcode <= 122) /* 'a' - 'z' */) {
        this.Lmodifiers ^= 0x10;  // Flip the 'shifted' bit, so it'll act as the opposite key.
        this.Lcode ^= 0x20; // Flips the 'upper' vs 'lower' bit for the base 'a'-'z' ASCII alphabetics.
      }
    }
  }
};