#include "q7816/m7816.h"
QVector<double> m7816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
