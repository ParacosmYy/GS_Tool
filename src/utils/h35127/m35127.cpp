#include "h35127/m35127.h"
QVector<double> m35127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
