#ifndef EvaValue_h
#define EvaValue_h

enum class EvaValueType {
    NUMBER,
};

struct EvaValue {
    EvaValueType type;
    union {
        double number;
    };
};

#define NUMBER(value) ((EvaValue){EvaValueType::NUMBER, .number = value})

#define AS_NUMBER(evaValue) ((double)(evaValue).number)

#endif
