#include "g29126/m29126.h"
QVector<double> m29126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
