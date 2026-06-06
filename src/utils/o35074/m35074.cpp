#include "o35074/m35074.h"
QVector<double> m35074::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
