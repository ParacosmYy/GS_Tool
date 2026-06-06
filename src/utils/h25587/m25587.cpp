#include "h25587/m25587.h"
QVector<double> m25587::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
