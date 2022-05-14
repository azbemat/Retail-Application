SET SERVEROUTPUT ON;

CREATE OR REPLACE PROCEDURE find_customer(customer_id IN NUMBER, found OUT NUMBER) AS
BEGIN
    SELECT
        cust_no INTO found
    FROM
        customers
    WHERE
        cust_no = customer_id;
    if(found > 0) THEN
        found := 1;
    END IF;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        found := 0;
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Error!');
END find_customer;

DECLARE
    test number;
BEGIN
    find_customer(10, test);
    DBMS_OUTPUT.PUT_LINE(test);
END;

CREATE OR REPLACE PROCEDURE find_product(product_id IN NUMBER, price OUT products.prod_sell%TYPE) AS
BEGIN
    SELECT
        prod_sell INTO price
    FROM
        products
    WHERE
        prod_no = product_id;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        price := 0;
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Error!');
END find_product;

DECLARE
    test DOUBLE;
BEGIN
    find_product(1000, test);
    DBMS_OUTPUT.PUT_LINE(test);
END;

CREATE OR REPLACE PROCEDURE add_order(customer_id IN NUMBER, new_order_id OUT NUMBER) AS
    orderDate orders.order_dt%TYPE;
BEGIN
    SELECT
        MAX(order_no)+1 INTO new_order_id
    FROM
        orders;
        
    SELECT
        TO_CHAR(SYSDATE, 'DD-Mon-YYYY') INTO orderDate
    FROM dual;

    INSERT INTO orders 
        (order_no, cust_no, status, rep_no, order_dt)
    VALUES
        (new_order_id, customer_id, 'C', 36, orderDate);
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        new_order_id := 0;
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Error!');
END add_order;

DECLARE
    test number;
BEGIN
    add_order(1001, test);
    DBMS_OUTPUT.PUT_LINE(test);
END;

ROLLBACK;

CREATE OR REPLACE PROCEDURE add_orderline(
    orderId IN orderlines.order_no%type,
    itemId IN orderlines.line_no%type,
    productId IN orderlines.prod_no%type,
    quantity IN orderlines.qty%type,
    price IN orderlines.price%type) AS
BEGIN
    INSERT INTO orderlines
        (order_no, line_no, prod_no, qty, price)
    VALUES
        (orderId, itemId, productId, quantity, price);
EXCEPTION
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Error!');
END add_orderline;

BEGIN
    add_orderline(330, 1, 1, 1, 1);
END;

ROLLBACK;