#include "a29520/m29520.h"
QVector<double> m29520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
