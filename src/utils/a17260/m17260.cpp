#include "a17260/m17260.h"
QVector<double> m17260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
