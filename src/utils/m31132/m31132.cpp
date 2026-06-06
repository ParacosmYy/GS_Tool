#include "m31132/m31132.h"
QVector<double> m31132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
