#include "i7848/m7848.h"
QVector<double> m7848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
