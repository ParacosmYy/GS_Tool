#include "h8587/m8587.h"
QVector<double> m8587::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
