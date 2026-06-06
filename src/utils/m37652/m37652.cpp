#include "m37652/m37652.h"
QVector<double> m37652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
