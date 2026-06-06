#include "a29000/m29000.h"
QVector<double> m29000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
