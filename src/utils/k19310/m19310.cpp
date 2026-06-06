#include "k19310/m19310.h"
QVector<double> m19310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
