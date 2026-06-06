#include "i28248/m28248.h"
QVector<double> m28248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
