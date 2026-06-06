#include "m21532/m21532.h"
QVector<double> m21532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
