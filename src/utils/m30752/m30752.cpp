#include "m30752/m30752.h"
QVector<double> m30752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
