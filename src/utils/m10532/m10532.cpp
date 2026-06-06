#include "m10532/m10532.h"
QVector<double> m10532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
