.section ".data"
    hello: .string "hello: "

.section ".text"
    adr x0, hello
    mov x1, #5
    mov x8, 2
    svc 0
func:
    b func