#include "i9908/m9908.h"
QVector<double> m9908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
