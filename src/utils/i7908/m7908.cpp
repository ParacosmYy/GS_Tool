#include "i7908/m7908.h"
QVector<double> m7908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
