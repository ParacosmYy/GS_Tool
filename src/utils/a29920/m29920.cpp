#include "a29920/m29920.h"
QVector<double> m29920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
