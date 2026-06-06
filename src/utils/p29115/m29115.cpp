#include "p29115/m29115.h"
QVector<double> m29115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
