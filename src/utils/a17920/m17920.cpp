#include "a17920/m17920.h"
QVector<double> m17920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
