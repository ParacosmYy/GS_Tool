#include "g29206/m29206.h"
QVector<double> m29206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
