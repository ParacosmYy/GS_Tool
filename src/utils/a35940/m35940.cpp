#include "a35940/m35940.h"
QVector<double> m35940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
