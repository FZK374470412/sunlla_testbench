/* MODULE(module, func) */
MODULE(I2C, sunlla_i2c_test)

/* CMD(module, op, in, out) 
 * @module: your test module
 * @op: the operation function name
 * @in: the max number of in parameters
 * @out: the max number of out parameters, not contain <status>
 */

/*I2C_OPEN <dev>
 * response: <status>
 */
CMD(I2C, OPEN, 1, 1)

/* I2C_START <dev>
 * response: <status>
 */
CMD(I2C, START, 1, 1)

/* I2C_CLOSE <dev>
 * response: <status>
 */
CMD(I2C, CLOSE, 1, 1)

/* ATTR(module, attr)
 * @module: your test module
 * @attr: attribute value
 */
ATTR(I2C, dev)
ATTR(I2C, checbak)
