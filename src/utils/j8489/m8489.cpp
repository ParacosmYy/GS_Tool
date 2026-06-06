#include "j8489/m8489.h"
QVector<double> m8489::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
