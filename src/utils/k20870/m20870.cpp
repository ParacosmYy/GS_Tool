#include "k20870/m20870.h"
QVector<double> m20870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
