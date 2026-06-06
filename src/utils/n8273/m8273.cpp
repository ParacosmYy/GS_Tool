#include "n8273/m8273.h"
QVector<double> m8273::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
