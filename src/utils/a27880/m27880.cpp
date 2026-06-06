#include "a27880/m27880.h"
QVector<double> m27880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
