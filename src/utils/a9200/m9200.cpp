#include "a9200/m9200.h"
QVector<double> m9200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
