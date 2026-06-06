#include "m29592/m29592.h"
QVector<double> m29592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
