#include "p29055/m29055.h"
QVector<double> m29055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
