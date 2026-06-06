#include "k35490/m35490.h"
QVector<double> m35490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
