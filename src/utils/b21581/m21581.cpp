#include "b21581/m21581.h"
QVector<double> m21581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
