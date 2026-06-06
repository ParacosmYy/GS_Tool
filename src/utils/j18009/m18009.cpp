#include "j18009/m18009.h"
QVector<double> m18009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
