#include "g27006/m27006.h"
QVector<double> m27006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
