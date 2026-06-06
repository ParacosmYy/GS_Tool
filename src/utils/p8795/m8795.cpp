#include "p8795/m8795.h"
QVector<double> m8795::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
