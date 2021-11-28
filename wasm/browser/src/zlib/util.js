/**
 * @fileoverview 雑多な関数群をまとめたモジュール実装.
 */
goog.provide("Zlib.Util");

goog.scope(function () {
  /**
   * Byte String から Byte Array に変換.
   * @param {!string} str byte string.
   * @return {!Array.<number>} byte array.
   */
  Zlib.Util.stringToByteArray = function (string_) {
    /** @type {!Array.<(string|number)>} */
    const temporary = [...string_];
    /** @type {number} */
    let index;
    /** @type {number} */
    let il;

    for (index = 0, il = temporary.length; index < il; index++) {
      temporary[index] = (temporary[index].charCodeAt(0) & 0xff) >>> 0;
    }

    return temporary;
  };

  // end of scope
});
