#include "i32608/m32608.h"
QVector<double> m32608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
