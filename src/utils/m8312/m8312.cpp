#include "m8312/m8312.h"
QVector<double> m8312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
