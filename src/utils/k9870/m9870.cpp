#include "k9870/m9870.h"
QVector<double> m9870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
