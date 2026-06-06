#include "k30270/m30270.h"
QVector<double> m30270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
