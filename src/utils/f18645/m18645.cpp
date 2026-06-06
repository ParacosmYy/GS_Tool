#include "f18645/m18645.h"
QVector<double> m18645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
