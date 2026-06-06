#include "a15760/m15760.h"
QVector<double> m15760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
