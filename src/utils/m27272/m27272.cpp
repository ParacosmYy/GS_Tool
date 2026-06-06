#include "m27272/m27272.h"
QVector<double> m27272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
