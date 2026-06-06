#include "l8011/m8011.h"
QVector<double> m8011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
