#include "k9370/m9370.h"
QVector<double> m9370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
