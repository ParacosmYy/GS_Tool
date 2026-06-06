#include "m32272/m32272.h"
QVector<double> m32272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
