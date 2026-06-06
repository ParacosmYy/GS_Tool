#include "j25489/m25489.h"
QVector<double> m25489::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
