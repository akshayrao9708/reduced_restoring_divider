import os
wire_count = 0

def set_up(n, outputfile):
    """
    Defines the wires and inputs and outputs for the restoring divider
    Arguments:
        n (int): Number of bits of the divider
        outputfile (str): Name of the output file of the generator
    """
    file = open(outputfile, 'a')
    file.write("module divider(q, rout, rin, div);\n")
    for i in range(n*(4*n+3*n)):# change to n*(4*n+3*n)
        file.write("  wire _%d_;\n" % (i))
    file.write("  wire [%d:0] r_0;\n" % (2*n-2))
    for i in range(n):
        file.write("  wire [%d:0] r_%d;\n" % (2*n-i-2, i+1)) # changed to 2*n-i-2
        file.write("  wire [%d:0] m_%d;\n" % (n-1, i+1))# changed to n 
        file.write("  wire [%d:0] sum_%d;\n" % (n-1, i+1))# changed to n 
        file.write("  wire [%d:0] inv_%d;\n" % (n-1, i+1))# changed to n 
    file.write("  wire zeroWire;\n")
    file.write("  wire oneWire;\n")
    file.write("  input [%d:0] rin;\n" % (2*n-3))
    file.write("  input [%d:0] div;\n" % (n-1)) # changed to n 
    file.write("  output [%d:0] q;\n" % (n-1))# changed to n-1
    file.write("  output [%d:0] rout;\n" % (n-1)) #changed to n-1
    file.write("  assign zeroWire = 1'b0 /*0*/;\n")# changed to n-1
    file.write("  assign oneWire = 1'b1 /*0*/;\n")
    file.close()

def shift(wire, b, length, num, name, outputfile, replace):
    """
    Left shift by b bits. The output wire of this function is called
    shift_(num)
    Arguments:
        wire(str): Name of the wire that is being shifted
        b(int): shift by b bits,
        length(int): number of bits of wire
        num(int): number of the call of the function shift with the corresponding name
        name (str): name the output wire should have
        outputfile (str): Name of the output file
        replace (int): max replace-level that can be used in this function"""
    file = open(outputfile, "a")
    for i in range(length):
        file.write("  assign %s_%d[%d] = %s[%d] /*%d*/;\n" %(name, num, b+i, wire, i, replace))
        replace -= 1
    for j in range(b):
        file.write("  assign %s_%d[%d] = zeroWire /*%d*/;\n" %(name, num, j, replace))
        replace -= 1
    file.close()

def full_adder(wire1, wire2, c_in, i, num, outputfile, replace, length):
    """
    Creates a full adder. The output wire is called sum_(num).
    Arguments:
        wire1(str): the first input wire
        wire2(str): the second input wire
        c_in(str): the carry-in
        i(int): the i-th bit of the inputs will be added
        num (int): The number of the adder
        outputfile (str): Name of the output file
        replace(int): Level for the replacement order during verification
        length(int): Length of the input wires
    """
    global wire_count
    file = open(outputfile, "a")
    file.write("  assign _%d_ = %s ^ %s /*%d*/;\n" %(wire_count, wire1, wire2,replace))
    wire_count += 1
    replace -= 1
    file.write("  assign sum_%d[%d] = _%d_ ^ %s /*%d*/;\n" %(num, i, wire_count -1 ,
                                                    c_in, replace))
    replace -= 1
    file.write("  assign _%d_ = _%d_ & %s /*%d*/;\n" % (wire_count, wire_count-1, c_in,replace))
    wire_count += 1
    replace -= 1
    file.write("  assign _%d_ = %s & %s /*%d*/;\n" % (wire_count, wire1, wire2, replace))
    wire_count += 1
    replace -= 1
    file.write("  assign _%d_ = _%d_ | _%d_ /*%d*/;\n" %(wire_count, wire_count-2,
                                                wire_count -1,replace))
    wire_count += 1
    replace -= 1
    file.close()

def full_adder_unsigned(wire1, wire2, c_in, i, num, outputfile, replace, length):
    """
    Creates a full adder. The output wire is called sum_(num).
    Arguments:
        wire1(str): the first input wire
        wire2(str): the second input wire
        c_in(str): the carry-in
        i(int): the i-th bit of the inputs will be added
        num (int): The number of the adder
        outputfile (str): Name of the output file
        replace(int): Level for the replacement order during verification
        length(int): Length of the input wires
    """
    global wire_count
    file = open(outputfile, "a")
    file.write("  assign _%d_ = %s ^ %s /*%d*/;\n" %(wire_count, wire1, wire2,replace))
    wire_count += 1
    replace -= 1
    file.write("  assign sum_%d[%d] = _%d_ ^ %s /*%d*/;\n" %(num, i, wire_count -1 ,
                                                    c_in, replace))
    replace -= 1
    file.write("  assign _%d_ = _%d_ & %s /*%d*/;\n" % (wire_count, wire_count-1, c_in,replace))
    wire_count += 1
    replace -= 1
    file.write("  assign _%d_ = %s & %s /*%d*/;\n" % (wire_count, wire1, wire2, replace))
    wire_count += 1
    replace -= 1
    if i != length-1:
        file.write("  assign _%d_ = _%d_ | _%d_ /*%d*/;\n" %(wire_count, wire_count-2,
                                                wire_count -1,replace))
    else:
        file.write("  assign sum_%d[%d] = _%d_ | _%d_ /*%d*/;\n" %(num, length, wire_count-2,
                                                wire_count -1,replace))
    wire_count += 1
    replace -= 1
    file.close()

def carry_ripple(start_1,start_2, in1, in2, cin, length, num, outputfile, replace):
    # 0,0, "inv_%d" % (i+1), "r_%d" % (i), "oneWire", 2*n-1+i, i+1, outputfile,
    #25*n+11*i-2+(n-i-1)*(34*n-1))

    """
    Creates a carry-ripple adder. The output wire of this function is called
    sum_(num).
    Arguments:
        start_1(int): start bit of in1 
        start_2(int): start bit of in2
        in1(str): the first input wire
        in2(str): the second input wire
        cin(str): the carry-in
        length(int): Length of the carry ripple adder
        num(int): Number of the call of the funtion
        outputfile(str): Name of the output file
        replace(int): Level for the replacement order during verification
    """
    for i in range(length):
        input1 = "%s[%d]" % (in1, i+start_1)
        input2 = "%s[%d]" % (in2, i+start_2)
        if i == 0:
            full_adder(input1, input2, cin, i, num, outputfile, replace-i, length)
        else:
            carry_in = "_%d_" % (wire_count -1)
            full_adder(input1, input2, carry_in, i, num, outputfile, replace-i, length)
        replace -= 5

def carry_ripple_unsigned(in1, in2, cin, length, num, outputfile, replace):
    """
    Creates a carry-ripple adder. The output wire of this function is called
    sum_(num).
    Arguments:
        in1(str): the first input wire
        in2(str): the second input wire
        cin(str): the carry-in
        length(int): Length of the carry ripple adder
        num(int): Number of the call of the funtion
        outputfile(str): Name of the output file
        replace(int): Level for the replacement order during verification
    """
    for i in range(length):
        input1 = "%s[%d]" % (in1, i)
        input2 = "%s[%d]" % (in2, i)
        if i == 0:
            full_adder_unsigned(input1, input2, cin, i, num, outputfile, replace, length)
        else:
            carry_in = "_%d_" % (wire_count -1)
            full_adder_unsigned(input1, input2, carry_in, i, num, outputfile, replace, length)
        replace -= 5

def extend(wire, length, new_length, num, name, outputfile, replace, vz=0,mux=2):
    
    """
    Fills a wire with zero if vz=0 and does sign extension if vz = 1. The
    output wire of the function is called a_(num)
    Arguments:
        wire(str): Name of the input wire
        length(int): Current length of the wire
        new_length(int): New length of the wire
        num(int): Number of the function call with the corresponding name
        name (str): name that the output wire should  have
        outputfile(str): Name of the outputfile
        replace (int): max replace-level that can be used in this function
        vz(int): 0 for filling with zeros, 1 for sign extension
    """
    file = open(outputfile, "a")
    if mux==0:
        for i in range(length):
            file.write("  assign %s_%d[%d] = %s[%d] /*%d*/;\n" % (name, num, i, wire, i, replace))
            replace -= 1
        if vz == 0: #zero extend
            for i in range(length, new_length):
                file.write("  assign %s_%d[%d] = zeroWire /*%d*/;\n" % (name, num, i, replace))
                replace -= 1
    else: 
        for i in range(length):
            file.write("  assign %s_%d[%d] = %s[%d] /*%d*/;\n" % (name, num, i, wire, new_length+i, replace))
            replace -= 1
    file.close()

def invert(in1, n, outputfile, num, replace):
    """Inverts all bits of the input and writes the lines in the outputfile.
       Arguments:
           in1 (str): name of the input that has to be inverted
           n (int): number of bits of the input
           outputfile (str): name of the file we write into
           num (int): number of the call of this function
           replace (int): max replace-level that can be used in this function
    """
    file = open(outputfile, 'a')
    for i in range(n):
        file.write("  assign inv_%d[%d] = ~%s[%d] /*%d*/;\n" % (num, i, in1, i, replace))
        replace -= 1
    file.close()

def define_second_input(n,num, outputfile, replace):
    """ Defines the second input of the adder that needs to be inverted.
        The shifted and negated version of d.
        Arguments:
            n (int): number of bits of the divider
            step (int): step in the divider, defines how much d needs to be shifted
            num (int): number of the call of the function
            carry (bool): tell wether the divider uses cout in CRA
            outputfile (str): name of the file we write into
            replace (int): max replace-level that can be used in this function
    """

    file = open(outputfile, 'a')
    #define the value of the bits
    for i in range(n-1):
        file.write("  assign inv_%d[%d] = ~div[%d] /*%d*/;\n" % (num, i,i, replace-5*i))
        replace -= 1
    file.write("assign inv_%d[%d] = oneWire /*%d*/;\n" % (num, n-1,replace-5*i))
    file.close()

def multiplexer(one_input, zero_input, decision, length, num, name, stage,outputfile, replace):
            #multiplexer("sum_%d" % (i+1), "m_%d" % (i+1), "q[%d]" % (n-1-i), 2*n+i, i+1, "r",
                   # outputfile, 11*n+4*i -1+ (n-i-1)*(34*n-1))
    """Creates a multiplexer with two inputs.
       Arguments:
           one_input (str): name of the input choosen when decision bit is 1
           zero_input (str): name of the input choosen when decision bit is 0
           decision (str): name of the decision bit
           length (int): bitlength of the inputs
           num (int): number of the call of this function with the corresponding name
           name (str): name that the output wire should have
           outputfile (str): name of the file we write into
           replace (int): max replace-level that ca be used in this function"""
    global wire_count
    file = open(outputfile, 'a')
    start_length = length-stage
    for i in range(0,length):
        file.write("  assign _%d_ = ~%s /*%d*/;\n" % (wire_count, decision, replace))
        wire_count += 1
        replace -= 1
        file.write("  assign _%d_ = %s[%d] & %s /*%d*/;\n" % (wire_count, one_input, i, decision,
                                                              replace))
        wire_count += 1
        replace -= 1
        file.write("  assign _%d_ = %s[%d] & _%d_ /*%d*/;\n" % (wire_count, zero_input, i,
                                                                wire_count-2, replace))
        wire_count += 1
        replace -= 1
        file.write("  assign %s_%d[%d] = _%d_ | _%d_ /*%d*/;\n" % (name, num,i+start_length, wire_count-1,
                                                                   wire_count-2, replace))
        replace-= 1
    for i in range(start_length):
        file.write("assign %s_%d[%d]= %s_%d[%d] /*%d*/;\n" %(name,num,i,name,num-1,i,replace))
        replace-= 1
    file.close()

def adder_module(outputfile, n):
    """creates a verilog file of a carry-ripple-adder for signed integers.
       Arguments:
           outputfile (str): name of the file we write into
           n (int): number of bits of the adder"""
    global wire_count
    wire_count = 0
    file = open(outputfile, 'a')
    #define wires, inputs and outputs in verilog file
    file.write("module adder(in1, in2, cin, out);\n")
    for i in range(5*(n+1)):
        file.write("  wire _%d_;\n" % (i))
    file.write("  wire [%d:0] sum_1;\n" % n)
    file.write("  wire zeroWire;\n")
    file.write("  wire oneWire;\n")
    file.write("  input [%d:0] in1;\n" % (n-1))
    file.write("  input [%d:0] in2;\n" % (n-1))
    file.write("  input cin;\n")
    file.write("  output [%d:0] out;\n" % (n))
    file.write("  assign zeroWire = 1'b0 /*0*/;\n")
    file.write("  assign oneWire = 1'b1 /*0*/;\n")
    file.close()
    #define the adder itself in the verilog file
    carry_ripple(0,0,"in1", "in2", "cin", n, 1, outputfile, 6*(n+1)+n)
    file = open(outputfile, 'a')
    #define the output
    for i in range(n+1):
        file.write("  assign out[%d] = sum_1[%d] /*%d*/;\n" % (i, i, i+1))
    file.write("endmodule")
    file.close()

def adder_unsigned_module(outputfile, n):
    """creates a verilog file of a carry-ripple-adder for unsigned integers.
       Arguments:
           outputfile (str): name of the file we write into
           n (int): number of bits of the adder"""
    global wire_count
    wire_count=0
    file = open(outputfile, 'a')
    #define wires, inputs and outputs in verilog file
    file.write("module adder(in1, in2, cin, out);\n")
    for i in range(5*(n+1)):
        file.write("  wire _%d_;\n" % (i))
    file.write("  wire [%d:0] sum_1;\n" % n)
    file.write("  wire zeroWire;\n")
    file.write("  wire oneWire;\n")
    file.write("  input [%d:0] in1;\n" % (n-1))
    file.write("  input [%d:0] in2;\n" % (n-1))
    file.write("  input cin;\n")
    file.write("  output [%d:0] out;\n" % (n))
    file.write("  assign zeroWire = 1'b0 /*0*/;\n")
    file.write("  assign oneWire = 1'b1 /*0*/;\n")
    file.close()
    #define the adder itself in the verilog file
    carry_ripple_unsigned("in1", "in2", "cin", n, 1, outputfile, 6*(n+1))
    file = open(outputfile, 'a')
    #define the output
    for i in range(n+1):
        file.write("  assign out[%d] = sum_1[%d] /*%d*/;\n" % (i, i, i+1))
    file.write("endmodule")
    file.close()

def multiplexer_complete_module(outputfile, n):
    """Creates a verilog file of a multiplexer with two inputs.
       Arguments:
           outputfile (str): name of the file we write into
           n (int): number of bits of the multiplexer"""
    global wire_count
    wire_count=0
    file = open(outputfile, 'a')
    #define all wires, inputs and outputs needed for the multiplexer
    file.write("module multiplexer(in1, in2, cin, out);\n")
    for i in range(3*n+1):
        file.write("  wire _%d_;\n" % (i))
    file.write("  wire [%d:0] mult_1;\n" % (n-1))
    file.write("  wire zeroWire;\n")
    file.write("  wire oneWire;\n")
    file.write("  input [%d:0] in1;\n" % (n-1))
    file.write("  input [%d:0] in2;\n" % (n-1))
    file.write("  input cin;\n")
    file.write("  output [%d:0] out;\n" % (n-1))
    file.write("  assign zeroWire = 1'b0 /*0*/;\n")
    file.write("  assign oneWire = 1'b1 /*0*/;\n")
    file.close()
    #write multiplexer in file
    multiplexer("in1", "in2", "cin", n, 1, "mult", outputfile, 5*n)
    file = open(outputfile, 'a')
    #define output bits of the multiplexer
    for i in range(n):
        file.write("  assign out[%d] = mult_1[%d] /*%d*/;\n" % (i, i, i+1))
    file.write("endmodule")
    file.close()

def main():
    #creates a verilog file with a customized name of a restoring divider with a customized size.
    replace_order = 0

    n = int(input("Please specify the number of bits the restoring divider should have:"))
    outputfile = input("Please specify the output file name:")
    if os.path.exists(outputfile):
        os.remove(outputfile)

    set_up(n, outputfile)
    # null anhängen an inputs (sext)
    extend("rin", 2*n-2, 2*n-1, 0, "r", outputfile, 34*n*n+29*n, vz=0, mux=0)
    #nicht mehr nötig
    #extend("div", n-1, n, 0, "d", outputfile, 34*n*n+27*n, vz=0)
    
    for i in range(n):
        # nicht mehr nötig, alternative für zweiten input
        #d shiften
        #shift("d_0", n-1-i, n, i+1, "d", outputfile, 27*n+10*i-3+(n-i-1)*(34*n-1))
        #d auffüllen wie r
        #extend("d_%d" % (i+1), 2*n-1-i, 2*n-1+i, i+1, "x", outputfile, 25*n+11*i-2+(n-i-1)*(34*n-1), vz=0) 
        #geshiftetes d negieren
        #invert("x_%d" % (i+1), 2*n-1+i, outputfile, i+1, 23*n+10*i-1+(n-i-1)*(34*n-1))
        
        #d negieren und mit 1 auffüllen
        define_second_input(n,i+1, outputfile, 25*n+11*i-1+(n-i-1)*(34*n-1))
        #addieren von neg d und r mit cin 1 (subtrahierer)
        carry_ripple(0,n-i-1, "inv_%d" % (i+1), "r_%d" % (i), "oneWire", n, i+1, outputfile, 25*n+11*i-2+(n-i-1)*(34*n-1))

        file = open(outputfile, 'a')
               
        file.write("  assign q[%d] = ~sum_%d[%d] /*%d*/;\n" %(n-1-i, i+1, n-1,13*n+5*i+(n-i-1)*(34*n-1)))

        file.close()
        # r sext
        extend("r_%d" % (i),n, n-i-1, i+1, "m", outputfile,13*n+5*i-1+(n-i-1)*(34*n-1),vz=1,mux=1)
        # mux
        multiplexer("sum_%d" % (i+1), "m_%d" % (i+1), "q[%d]" % (n-1-i),n, i+1, "r",i+1,
                    outputfile, 11*n+4*i -1+ (n-i-1)*(34*n-1))
        
      
    file = open(outputfile, 'a')
    for i in range(n):
        file.write("  assign rout[%d] = r_%d[%d] /*%d*/;\n" % (i, n, i, i))
    file.write("endmodule")
    file.close()
    return

if __name__ == main():
    main()
