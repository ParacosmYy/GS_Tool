#include "m24112/m24112.h"
QVector<double> m24112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
