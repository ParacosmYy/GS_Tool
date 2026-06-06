#include "a21700/m21700.h"
QVector<double> m21700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
