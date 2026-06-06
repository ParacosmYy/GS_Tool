#include "m21112/m21112.h"
QVector<double> m21112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
