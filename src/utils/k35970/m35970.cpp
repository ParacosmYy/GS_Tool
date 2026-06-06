#include "k35970/m35970.h"
QVector<double> m35970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
