#include "m13132/m13132.h"
QVector<double> m13132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
