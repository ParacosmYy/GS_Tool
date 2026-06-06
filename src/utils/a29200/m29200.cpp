#include "a29200/m29200.h"
QVector<double> m29200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
